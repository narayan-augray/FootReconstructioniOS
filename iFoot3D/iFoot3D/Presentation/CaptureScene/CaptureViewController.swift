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
        setupARSessionManager()
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        contentView.startSession()
    }
}

// MARK: - Private
private extension CaptureViewController {
    func setupARSessionManager() {
        viewModel.arSessionManager.setSceneView(view: contentView.getSceneView())
        contentView.setupSessionDelegate(delegate: viewModel.arSessionManager)
    }
    
    func setupBindings() {
        contentView.actionPublisher
            .sink { [unowned self] (action) in
                switch action {
                case .selectPossition(let position):
                    viewModel.selectFootPosition(position: position)
                
                case .capture:
                    #warning("to do")
                    
                case .error(let message):
                    viewModel.handleError(messsage: message)
                }
            }
            .store(in: &cancellables)
        
        viewModel.arSessionManager.eventPublisher
            .receive(on: DispatchQueue.main)
            .sink { [unowned self] (event) in
                switch event {
                case .coachingDeactivated:
                    contentView.setState(state: .selectPosition)
                }
            }
            .store(in: &cancellables)
        
        viewModel.arSessionManager.sceneCenterPublisher
            .throttle(for: .milliseconds(100), scheduler: DispatchQueue.main, latest: true)
            .sink { [unowned self] (position) in
                contentView.updateFootNodePosition(position: position)
            }
            .store(in: &cancellables)
    }
}
