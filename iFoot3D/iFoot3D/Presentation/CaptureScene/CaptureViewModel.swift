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
    case capturePositions(positions: [SCNVector3])
}

final class CaptureViewModel: BaseViewModel {
    // MARK: - Properties
    private var footPosition: SCNVector3?
    
    // MARK: - Published
    @Published private(set) var capturePositions: [SCNVector3] = []
    
    // MARK: - Services
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
    
    func selectFootPosition(position: SCNVector3) {
        footPosition = position
        generateCapturePositions()
    }
}

// MARK: - Private
private extension CaptureViewModel {
    func generateCapturePositions() {
        guard let footPosition = footPosition else {
            capturePositions = []
            return
        }
        
        let numberOfPoints: Float = 8.0
        let radius: Float = 0.3
        let yDistance: Float = 0.5
        let circle = 2 * Float.pi
        
        var result: [SCNVector3] = []
        
        for angle in stride(from: 0.0, to: circle, by: circle / numberOfPoints) {
            let x = footPosition.x + radius * cos(angle)
            let z = footPosition.z + radius * sin(angle)
            result.append(.init(x, footPosition.y + yDistance, z))
        }
        
        capturePositions = result
    }
}
