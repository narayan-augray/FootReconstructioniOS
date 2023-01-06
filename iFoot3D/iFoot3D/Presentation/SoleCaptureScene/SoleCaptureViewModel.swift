//
//  SoleCaptureViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import Combine

final class SoleCaptureViewModel: BaseViewModel {
    // MARK: - Properties
    private let outputs: [CaptureProcessedOutput]
    
    // MARK: - Services
    let arSessionManager: ARSessionManager
    let speechRecognier: SpeechRecognizer
    
    // MARK: - Transition
    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<SoleCaptureTransition, Never>()
    
    // MARK: - Init
    init(outputs: [CaptureProcessedOutput],
         arSessionManager: ARSessionManager,
         speechRecognier: SpeechRecognizer) {
        self.outputs = outputs
        self.arSessionManager = arSessionManager
        self.speechRecognier = speechRecognier
        super.init()
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
}
