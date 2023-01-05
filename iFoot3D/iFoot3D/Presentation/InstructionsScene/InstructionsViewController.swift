//
//  InstructionsViewController.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import UIKit

final class InstructionsViewController: BaseViewController<InstructionsViewModel> {
    // MARK: - Views
    private let contentView = InstructionsView()
    
    // MARK: - Lifecycle
    override func loadView() {
        view = contentView
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        navigationController?.navigationBar.isHidden = true
        setupBindings()
    }
}

// MARK: - Private
private extension InstructionsViewController {
    func setupBindings() {
        contentView.actionPublisher
            .sink { [unowned self] action in
                switch action {
                case .continue:
                    viewModel.continue()
                }
            }
            .store(in: &cancellables)
    }
}
