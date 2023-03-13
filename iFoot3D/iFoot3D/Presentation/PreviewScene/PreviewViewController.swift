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
                    
                    viewModel.archive()
                    
                case .close:
                    viewModel.close()
                }
            }
            .store(in: &cancellables)
        
        viewModel.zipFileUrlPublisher
            .receive(on: DispatchQueue.main)
            .sink { [unowned self] (zipUrl) in
                guard let zipUrl = zipUrl else {
                    return
                }
                shareFile(fileUrl: zipUrl)
            }
            .store(in: &cancellables)
    }
    
    func setupPreview() {
        guard let objectUrl = viewModel.objectUrl else {
            return
        }
        contentView.setupPreview(objectUrl: objectUrl)
    }
    
    func shareFile(fileUrl: URL) {
        let activityViewController = ActivityViewController(activityItems: [fileUrl], applicationActivities: nil)
        activityViewController.onDismiss = { [weak self] in
            self?.viewModel.deleteFiles()
            self?.viewModel.success()
        }
        present(activityViewController, animated: true) { [weak self] in
            self?.viewModel.isLoadingSubject.send(false)
        }
    }
}
