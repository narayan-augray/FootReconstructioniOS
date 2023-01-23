//
//  SuccessViewController.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import UIKit
import Zip

final class SuccessViewController: BaseViewController<SuccessViewModel> {
    // MARK: - Views
    private let contentView = SuccessView()
    
    // MARK: - Lifecycle
    override func loadView() {
        view = contentView
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        setupBindings()
    }
}

// MARK: - Private
private extension SuccessViewController {
    func setupBindings() {
        contentView.actionPublisher
            .sink { [unowned self] (action) in
                switch action {
                case .export:
                    export(outputs: viewModel.outputs)
                }
            }
            .store(in: &cancellables)
    }
}

// MARK: - Export
private extension SuccessViewController {
    func export(outputs: [CaptureProcessedOutput]) {
        var fileUrls: [URL?] = outputs.getFilesUrls()
        
        viewModel.isLoadingSubject.send(true)
        
        getArchive(files: fileUrls) { [weak self] (zipFileUrl) in
            guard let zipFileUrl = zipFileUrl else {
                self?.deleteFiles(fileUrls: fileUrls)
                self?.viewModel.navigateBack()
                return
            }
            
            fileUrls.append(zipFileUrl)
            
            self?.shareFile(fileUrl: zipFileUrl) {
                self?.deleteFiles(fileUrls: fileUrls)
                self?.viewModel.navigateBack()
            }
        }
    }
    
    func getArchiveName() -> String {
        let dateFormatter = DateFormatter()
        dateFormatter.dateFormat = "yyyy-MM-dd_HH:mm"
        return "\(UIDevice.modelName)_\(dateFormatter.string(from: Date()))"
    }
    
    func deleteFiles(fileUrls: [URL?]) {
        let fileManager = FileManager.default
        for fileUrl in fileUrls where fileUrl != nil {
            try? fileManager.removeItem(at: fileUrl!)
        }
    }
    
    func getArchive(files: [URL?], completion: @escaping (URL?) -> ()) {
        DispatchQueue.global().async { [weak self] in
            guard let self = self else { return }
            let archiveName = self.getArchiveName()
            do {
                let zipFilePath = try Zip.quickZipFiles(files.compactMap({ $0 }), fileName: archiveName)
                DispatchQueue.main.async {
                    completion(zipFilePath)
                }
            } catch {
                log.error(error: error)
                
                DispatchQueue.main.async {
                    completion(nil)
                }
            }
        }
    }
    
    func shareFile(fileUrl: URL, completion: @escaping () -> ()) {
        var filesToShare = [Any]()
        filesToShare.append(fileUrl)
        let activityViewController = ActivityViewController(activityItems: filesToShare, applicationActivities: nil)
        activityViewController.onDismiss = {
            completion()
        }
        present(activityViewController, animated: true) { [weak self] in
            self?.viewModel.isLoadingSubject.send(false)
        }
    }
}
