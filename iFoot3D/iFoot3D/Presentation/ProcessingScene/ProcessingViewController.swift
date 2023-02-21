//
//  ProcessingViewController.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 02.02.2023.
//

import UIKit

final class ProcessingViewController: BaseViewController<ProcessingViewModel> {
    // MARK: - Views
    private let contentView = ProcessingView()
    
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
private extension ProcessingViewController {
    func setupBindings() {
        viewModel.actionPublisher
            .sink { [unowned self] (action) in
                switch action {
                case .reconstructionFailed:
                    contentView.showFailure()
                    
                    restartAfterDelay()
                }
            }
            .store(in: &cancellables)
    }
    
    func restartAfterDelay() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 2.0) { [weak self] in
            self?.viewModel.capture()
        }
    }
}
