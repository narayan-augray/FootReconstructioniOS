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
                    print("to do")
                    
                case .close:
                    viewModel.navigateBack()
                }
            }
            .store(in: &cancellables)
    }
    
    func setupPreview() {
        contentView.setupPreview(objectUrl: viewModel.objectUrl)
    }
}
