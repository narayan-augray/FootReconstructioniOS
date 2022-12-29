//
//  CaptureViewController.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import UIKit

final class CaptureViewController: BaseViewController<CaptureViewModel> {
    // MARK: - Properties
    private var isCapturing: Bool = false
    
    // MARK: - Views
    private let contentView = CaptureView()
    
    // MARK: - Lifecycle
    override func loadView() {
        view = contentView
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        setupBindings()
        setupARSessionDelegate()
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        contentView.startSession()
    }
}

// MARK: - Private
private extension CaptureViewController {
    func setupARSessionDelegate() {
        contentView.setupSessionDelegate(delegate: viewModel.arSessionManager)
    }
    
    func setupBindings() {
        contentView.actionPublisher
            .sink { [unowned self] action in
                switch action {
                case .capture:
                    #warning("to do")
                    
                case .error(let message):
                    viewModel.handleError(messsage: message)
                }
            }
            .store(in: &cancellables)
    }
}
