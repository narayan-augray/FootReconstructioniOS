//
//  ARSessionManager.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import ARKit
import Combine

protocol ARSessionManager: ARSessionCombinedDelegate {
    // MARK: - Properties
    var sceneCenterPublisher: AnyPublisher<SCNVector3?, Never> { get }
    var eventPublisher: AnyPublisher<ARSessionManagerEvent, Never> { get }
    
    // MARK: - Funcs
    func setSceneView(view: ARSCNView)
}

final class ARSessionManagerImpl: NSObject, ARSessionManager {
    // MARK: - Properties
    private weak var sceneView: ARSCNView?
    
    // MARK: - Publishers
    private(set) lazy var sceneCenterPublisher = sceneCenterSubject.eraseToAnyPublisher()
    private let sceneCenterSubject = CurrentValueSubject<SCNVector3?, Never>(nil)
    
    private(set) lazy var eventPublisher = eventSubject.eraseToAnyPublisher()
    private let eventSubject = PassthroughSubject<ARSessionManagerEvent, Never>()
    
    // MARK: - Init
    override init() {
        super.init()
    }
    
    // MARK: - Public
    func setSceneView(view: ARSCNView) {
        sceneView = view
    }
}

// MARK: - Private
private extension ARSessionManagerImpl {
    func calculateSceneCenter() {
        DispatchQueue.main.async { [weak self] in
            guard let sceneView = self?.sceneView else { return }
            self?.sceneCenterSubject.value = sceneView.realWorldPosition(for: sceneView.center)
        }
    }
}

// MARK: - ARSessionDelegate
extension ARSessionManagerImpl {
    func session(_ session: ARSession, didUpdate frame: ARFrame) {
        eventSubject.send(.newFrame(frame: frame))
    }
}

// MARK: - ARSCNViewDelegate
extension ARSessionManagerImpl {
    func renderer(_ renderer: SCNSceneRenderer, updateAtTime time: TimeInterval) {
        calculateSceneCenter()
    }
}

// MARK: - ARCoachingOverlayViewDelegate
extension ARSessionManagerImpl {
    func coachingOverlayViewDidDeactivate(_ coachingOverlayView: ARCoachingOverlayView) {
        eventSubject.send(.coachingDeactivated)
    }
}
