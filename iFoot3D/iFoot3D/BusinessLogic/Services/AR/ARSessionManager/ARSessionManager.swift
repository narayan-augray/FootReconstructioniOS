//
//  ARSessionManager.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import ARKit
import Combine

protocol ARSessionManager: ARSessionDelegate {
    
}

final class ARSessionManagerImpl: NSObject, ARSessionManager {
    // MARK: - Publisher
    private(set) lazy var eventPublisher = eventSubject.eraseToAnyPublisher()
    private let eventSubject = PassthroughSubject<ARSessionManagerEvent, Never>()
}

// MARK: - ARSessionDelegate
extension ARSessionManagerImpl {
    func session(_ session: ARSession, didUpdate frame: ARFrame) {
        #warning("to do")
    }
}
