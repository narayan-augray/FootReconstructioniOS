//
//  FailureViewController.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 26.05.2023.
//

import UIKit

final class FailureViewController: BaseViewController<FailureViewModel> {
    // MARK: - Views
    private let contentView = FailureView()
    
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
private extension FailureViewController {
    func setupBindings() {
        contentView.actionPublisher
            .sink { [unowned self] (action) in
                switch action {
                case .tryAgain:
                    viewModel.tryAgain()
                }
            }
            .store(in: &cancellables)
    }
}
