//
//  CaptureService.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import ARKit
import Combine

protocol CaptureService {
    // MARK: - Properties
    var eventPublisher: AnyPublisher<CaptureServiceEvent, Never> { get }
    
    // MARK: - Funcs
    func generateCapturePositions(with center: SCNVector3)
    func handleNewFrame(frame: ARFrame)
}

final class CaptureServiceImpl: CaptureService {
    // MARK: - Properties
    private var footPosition: SCNVector3!
    private var capturePositions: [CapturePosition] = [] {
        didSet {
            eventSubject.send(.capturePositions(positions: capturePositions))
        }
    }
    
    // MARK: - Publishers
    private(set) lazy var eventPublisher = eventSubject.eraseToAnyPublisher()
    private let eventSubject = PassthroughSubject<CaptureServiceEvent, Never>()
}

// MARK: - Positions generation
extension CaptureServiceImpl {
    func generateCapturePositions(with center: SCNVector3) {
        footPosition = center
        
        var result: [CapturePosition] = []
        
        for angle in stride(from: 0.0, to: Constant.circle, by: Constant.circle / Constant.numberOfPositions) {
            let x = footPosition.x + Constant.radius * cos(angle)
            let z = footPosition.z + Constant.radius * sin(angle)
            
            var yRotation = angle
            if angle == 3 * .pi / 2 {
                yRotation = 0
            } else if angle.truncatingRemainder(dividingBy: .pi / 2) == 0 {
                yRotation -= .pi / 2
            }
            
            var xRotation = Constant.xRotation
            if angle == 5 * .pi / 4 || angle == .pi / 4 || angle == .pi / 2 {
                xRotation *= -1
            }
            
            result.append(.init(position: .init(x: x, y: footPosition.y + Constant.yDistance, z: z),
                                rotation: .init(xRotation, yRotation, 0)))
        }
        
        capturePositions = result
    }
}

// MARK: - Frames observing
extension CaptureServiceImpl {
    func handleNewFrame(frame: ARFrame) {
        guard
        print(frame.camera.transform)
        #warning("to do")
    }
}

// MARK: - Constants
private struct Constant {
    static let circle = 2 * Float.pi
    static let numberOfPositions: Float = 8.0
    static let radius: Float = 0.3
    static let yDistance: Float = 0.5
    static let xRotation: Float = .pi / 6
}
