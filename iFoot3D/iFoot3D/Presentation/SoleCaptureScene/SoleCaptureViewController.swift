//
//  SoleCaptureViewController.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import UIKit
import AVFoundation

final class SoleCaptureViewController: BaseViewController<SoleCaptureViewModel> {
    // MARK: - Views
    private let contentView = SoleCaptureView()
    
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
private extension SoleCaptureViewController {
    func setupARSessionManager() {
        viewModel.arSessionManager.setSceneView(view: contentView.getSceneView())
        contentView.setupSessionDelegate(delegate: viewModel.arSessionManager)
    }
    
    func setupBindings() {
        contentView.actionPublisher
            .sink { [unowned self] action in
                switch action {
                case .error(let message):
                    viewModel.handleError(messsage: message)
                    
                case .complete:
                    viewModel.complete()
                }
            }
            .store(in: &cancellables)
        
        viewModel.arSessionManager.eventPublisher
            .receive(on: DispatchQueue.main)
            .sink { [unowned self] (event) in
                switch event {
                case .coachingDeactivated:
                    contentView.showFootOverlay()
                    
                default:
                    break
                }
            }
            .store(in: &cancellables)
        
        viewModel.captureService.eventPublisher
            .receive(on: DispatchQueue.main)
            .sink { [unowned self] (event) in
                switch event {
                case .captureOutput(let output, _):
                    playSound()
                
                    viewModel.processOutput(output: output)
                    
                    contentView.animate()

                default:
                    break
                }
            }
            .store(in: &cancellables)
        
        viewModel.capturedFramesPublisher
            .sink { [unowned self] (count) in
                contentView.updateCountValue(count: count)
                
                if count == CaptureConstants.requiredSoleImagesCount {
                    contentView.showCompleteButton()
                }
            }
            .store(in: &cancellables)
    }
}

// MARK: - Helpers
private extension SoleCaptureViewController {
    func playSound() {
        AudioServicesPlaySystemSound(1108)
    }
}
