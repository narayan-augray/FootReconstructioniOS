//
//  FootCaptureViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import Foundation
import ARKit
import Combine

final class FootCaptureViewModel: BaseViewModel {
    // MARK: - Services
    let arSessionManager: ARSessionManager
    let captureService: CaptureService
    let captureOutputManager: CaptureOutputManager
    
    // MARK: - Transition
    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<FootCaptureTransition, Never>()
    
    // MARK: - Init
    init(arSessionManager: ARSessionManager,
         captureService: CaptureService,
         captureOutputManager: CaptureOutputManager) {
        self.arSessionManager = arSessionManager
        self.captureService = captureService
        self.captureOutputManager = captureOutputManager
        
        super.init()
        
        setupServices()
        setupBindings()
    }
    
    // MARK: - Public
    func handleError(messsage: String) {
        errorSubject.send(messsage)
    }
    
    func selectFootPosition(position: SCNVector3) {
        captureService.generateCapturePositions(with: position)
    }
    
    func processOutput(output: CaptureOutput) {
        captureOutputManager.processOutput(output: output)
        
        if captureOutputManager.getCaputredFrames() == CaptureConstants.requiredImagesCount {
            isLoadingSubject.send(true)
            captureOutputManager.finishProcessing()
        }
    }
}

// MARK: - Private
private extension FootCaptureViewModel {
    func setupServices() {
        captureOutputManager.reset()
    }
    
    func setupBindings() {
        arSessionManager.eventPublisher
            .sink { [unowned self] (event) in
                switch event {
                case .newFrame(let frame):
                    captureService.handleNewFootFrame(frame: frame)
                    
                default:
                    break
                }
            }
            .store(in: &cancellables)
        
        captureOutputManager.eventPublisher
            .receive(on: DispatchQueue.main)
            .sink { [unowned self] (event) in
                switch event {
                case .processedOutputs(let outputs):
                    transitionSubject.send(.instructions(outputs: outputs))
                }
            }
            .store(in: &cancellables)
    }
}
