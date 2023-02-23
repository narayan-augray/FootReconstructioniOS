//
//  FootCaptureViewController.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import UIKit
import AVFoundation

final class FootCaptureViewController: BaseViewController<FootCaptureViewModel> {
    // MARK: - Properties
    private var isCapturing: Bool = false
    
    // MARK: - Views
    private let contentView = FootCaptureView()
    
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
private extension FootCaptureViewController {
    func setupARSessionManager() {
        viewModel.arSessionManager.setSceneView(view: contentView.getSceneView())
        contentView.setupSessionDelegate(delegate: viewModel.arSessionManager)
    }
    
    func setupBindings() {
        contentView.actionPublisher
            .sink { [unowned self] (action) in
                switch action {
                case .selectPossition(let position, let angle):
                    viewModel.selectFootPosition(position: position, angle: angle)
                   
                case .error(let message):
                    viewModel.handleError(messsage: message)
                }
            }
            .store(in: &cancellables)
        
        viewModel.captureService.eventPublisher
            .receive(on: DispatchQueue.main)
            .sink { [unowned self] (event) in
                switch event {
                case .capturePositions(let positions):
                    contentView.createPhoneNodes(positions: positions)
                    
                case .captureOutput(let output, let capturePositionId):
                    playSound()
                    generateHapticFeedback()
                
                    if let capturePositionId = capturePositionId {
                        contentView.highlightPhoneNode(id: capturePositionId)
                    }
                    
                    viewModel.processOutput(output: output)
                }
            }
            .store(in: &cancellables)
        
        viewModel.arSessionManager.eventPublisher
            .receive(on: DispatchQueue.main)
            .sink { [unowned self] (event) in
                switch event {
                case .coachingDeactivated:
                    contentView.setState(state: .selectPosition)
                
                default:
                    break
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

// MARK: - Helpers
private extension FootCaptureViewController {
    func generateHapticFeedback() {
        let generator = UIImpactFeedbackGenerator(style: .medium)
        generator.impactOccurred()
    }
    
    func playSound() {
        AudioServicesPlaySystemSound(1108)
    }
}
