//
//  CaptureViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import Combine

final class CaptureViewModel: BaseViewModel {
    // MARK: - Properties
    let arSessionManager: ARSessionManager
    
    // MARK: - Transition
    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<CaptureTransition, Never>()
    
    // MARK: - Init
    init(arSessionManager: ARSessionManager) {
        self.arSessionManager = arSessionManager
        super.init()
    }
    
    // MARK: - Public
    func handleError(messsage: String) {
        errorSubject.send(messsage)
    }
}
