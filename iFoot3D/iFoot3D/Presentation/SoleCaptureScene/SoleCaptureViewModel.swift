//
//  SoleCaptureViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import ARKit
import Combine

final class SoleCaptureViewModel: BaseViewModel {
    // MARK: - Properties
    private var outputs: [CaptureProcessedOutput]
    private var capture: Bool = false
    
    // MARK: - Services
    let arSessionManager: ARSessionManager
    let speechRecognier: SpeechRecognizer
    let captureService: CaptureService
    let captureOutputManager: CaptureOutputManager
    
    // MARK: - Publisheres
    private(set) lazy var capturedFramesPublisher = capturedFramesSubject.eraseToAnyPublisher()
    private let capturedFramesSubject = CurrentValueSubject<Int, Never>(0)
    
    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<SoleCaptureTransition, Never>()
    
    // MARK: - Init
    init(outputs: [CaptureProcessedOutput],
         arSessionManager: ARSessionManager,
         speechRecognier: SpeechRecognizer,
         captureService: CaptureService,
         captureOutputManager: CaptureOutputManager) {
        self.outputs = outputs
        self.arSessionManager = arSessionManager
        self.speechRecognier = speechRecognier
        self.captureService = captureService
        self.captureOutputManager = captureOutputManager
        super.init()
        setupBindings()
    }
    
    // MARK: - Lifecycle
    override func onViewDidAppear() {
        super.onViewDidAppear()
        speechRecognier.startRecognition()
    }
    
    override func onViewDidDisappear() {
        super.onViewDidDisappear()
        speechRecognier.stopRecognition()
    }
    
    // MARK: - Public
    func handleError(messsage: String) {
        errorSubject.send(messsage)
    }
    
    func processOutput(output: CaptureOutput) {
        captureOutputManager.processOutput(output: output)
    }
    
    func complete() {
        isLoadingSubject.send(true)
        captureOutputManager.finishProcessing()
    }
}

// MARK: - Private
private extension SoleCaptureViewModel {
    func setupBindings() {
        arSessionManager.eventPublisher
            .sink { [unowned self] (event) in
                switch event {
                case .newFrame(let frame):
                    if frame.capturedDepthData != nil, capture {
                        captureService.handleNewSoleFrame(frame: frame)
                        
                        capture = false
                        
                        capturedFramesSubject.value += 1
                    }
                    
                default:
                    break
                }
            }
            .store(in: &cancellables)
        
        speechRecognier.eventPublisher
            .sink { [unowned self] (event) in
                switch event {
                case .authorizationFailed, .recognitionFailed:
                    errorSubject.send("Speech recognition failed")
                    
                case .recognized(let text):
                    if snapCommandRecognized(text: text) {
                        capture = true
                    }
                }
            }
            .store(in: &cancellables)
        
        captureOutputManager.eventPublisher
            .receive(on: DispatchQueue.main)
            .sink { [unowned self] (event) in
                switch event {
                case .processedOutputs(let soleOutputs):
                    outputs.append(contentsOf: soleOutputs)
                    transitionSubject.send(.success(outputs: outputs))
                }
            }
            .store(in: &cancellables)
    }
    
    func snapCommandRecognized(text: String) -> Bool {
        return text.contains(CaptureConstants.snapCommand)
    }
}
