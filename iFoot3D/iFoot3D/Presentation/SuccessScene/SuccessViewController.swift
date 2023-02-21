//
//  SuccessViewController.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import UIKit

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
                case .scanAgain:
                    viewModel.scanAgain()
                }
            }
            .store(in: &cancellables)
    }
}
