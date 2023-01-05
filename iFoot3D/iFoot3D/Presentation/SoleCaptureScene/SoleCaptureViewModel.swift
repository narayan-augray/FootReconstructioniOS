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
    
    // MARK: - Transition
    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<SoleCaptureTransition, Never>()
    
    // MARK: - Init
    init(outputs: [CaptureProcessedOutput],
         arSessionManager: ARSessionManager) {
        self.outputs = outputs
        self.arSessionManager = arSessionManager
        super.init()
    }
}
