//
//  CaptureViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import Foundation
import ARKit
import Combine

enum CaptureViewModelEvent {
    case coaching
}

final class CaptureViewModel: BaseViewModel {
    // MARK: - Properties
    private var selectedFootPosition: SCNVector3?
    
    // MARK: - Services
    let arSessionManager: ARSessionManager
    
    // MARK: - Transition
    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<CaptureTransition, Never>()
    
    // MARK: - Init
    init(arSessionManager: ARSessionManager) {
        self.arSessionManager = arSessionManager
        super.init()
        setupARSessionManagerBindings()
    }
    
    // MARK: - Public
    func handleError(messsage: String) {
        errorSubject.send(messsage)
    }
    
    func selectFootPosition(position: SCNVector3) {
        selectedFootPosition = position
    }
}

// MARK: - Setup Bindings
private extension CaptureViewModel {
    func setupARSessionManagerBindings() {
        
    }
}
