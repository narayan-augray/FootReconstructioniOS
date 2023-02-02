//
//  PreviewViewController.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 02.02.2023.
//

import UIKit

final class PreviewViewController: BaseViewController<PreviewViewModel> {
    // MARK: - Views
    private let contentView = PreviewView()
    
    // MARK: - Lifecycle
    override func loadView() {
        view = contentView
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        setupBindings()
        setupPreview()
    }
}

// MARK: - Private
private extension PreviewViewController {
    func setupBindings() {
        contentView.actionPublisher
            .sink { [unowned self] (action) in
                switch action {
                case .export:
                    viewModel.isLoadingSubject.send(true)
                    viewModel.zipOutput()
                    
                case .close:
                    viewModel.close()
                }
            }
            .store(in: &cancellables)
        
        viewModel.actionPublisher
            .receive(on: DispatchQueue.main)
            .sink { [unowned self] (action) in
                switch action {
                case .outputZipped(let zipUrl):
                    shareFile(zipUrl: zipUrl)
                }
            }
            .store(in: &cancellables)
    }
    
    func setupPreview() {
        contentView.setupPreview(objectUrl: viewModel.objectUrl)
    }
    
    func shareFile(zipUrl: URL) {
        let filesToShare: [URL] = [zipUrl]
        let activityViewController = ActivityViewController(activityItems: filesToShare, applicationActivities: nil)
        activityViewController.onDismiss = { [weak self] in
            self?.viewModel.deleteFiles()
            self?.viewModel.success()
        }
        present(activityViewController, animated: true) { [weak self] in
            self?.viewModel.isLoadingSubject.send(false)
        }
    }
}
