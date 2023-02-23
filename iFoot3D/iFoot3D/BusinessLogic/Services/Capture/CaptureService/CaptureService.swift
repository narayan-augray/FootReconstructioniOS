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
    func generateCapturePositions(with center: SCNVector3, rotationAngle: Float)
    func handleNewFootFrame(frame: ARFrame)
    func handleNewSoleFrame(frame: ARFrame)
}

final class CaptureServiceImpl: CaptureService {
    // MARK: - Properties
    private var footPosition: SCNVector3!
    private var capturePositions: [CapturePosition] = []
    
    // MARK: - Publishers
    private(set) lazy var eventPublisher = eventSubject.eraseToAnyPublisher()
    private let eventSubject = PassthroughSubject<CaptureServiceEvent, Never>()
}

// MARK: - Positions generation
extension CaptureServiceImpl {
    func generateCapturePositions(with center: SCNVector3, rotationAngle: Float) {
        footPosition = center
        
        var result: [CapturePosition] = []
        
        var currentIndex: Int = 1
        
        let numberOfPositions = Float(CaptureConstants.requiredImagesCount)
        
        for angle in stride(from: 0.0, to: Constant.circle, by: Constant.circle / numberOfPositions) {
            let shiftedAngle = angle - rotationAngle
            
            let x = footPosition.x + Constant.radius * cos(shiftedAngle)
            let z = footPosition.z + Constant.radius * sin(shiftedAngle)
            
            var yRotation = angle + rotationAngle
            if angle == 3 * .pi / 2 {
                yRotation = rotationAngle
            } else if angle.truncatingRemainder(dividingBy: .pi / 2) == 0 {
                yRotation -= .pi / 2
            }
            
            var xRotation = Constant.xRotation
            if angle == 5 * .pi / 4 || angle == .pi / 4 || angle == .pi / 2 {
                xRotation *= -1
            }
            
            currentIndex += 1
           
            if currentIndex >= CaptureConstants.requiredImagesCount {
                currentIndex -= CaptureConstants.requiredImagesCount
            }
            
            result.append(.init(index: currentIndex,
                                position: .init(x: x, y: footPosition.y + Constant.yDistance, z: z),
                                rotation: .init(xRotation, yRotation, 0)))
        }
        
        capturePositions = result
        eventSubject.send(.capturePositions(positions: result))
    }
}

// MARK: - Foot frame
extension CaptureServiceImpl {
    func handleNewFootFrame(frame: ARFrame) {
        guard
            let index = currentCaptureTransform(camera: frame.camera),
            let originalPixelBuffer = frame.capturedImage.copy(),
            let depthPixelBuffer = frame.sceneDepth?.depthMap.copy()
        else {
            return
        }
        let output = CaptureOutput(index: capturePositions[index].index,
                                   originalPixelBuffer: originalPixelBuffer,
                                   depthPixelBuffer: depthPixelBuffer,
                                   intrinsics: frame.camera.intrinsics,
                                   transform: frame.camera.transform)
        eventSubject.send(.captureOutput(output: output, capturePositionId: capturePositions[index].id))
        capturePositions.remove(at: index)
    }
}

// MARK: - Sole frame
extension CaptureServiceImpl {
    func handleNewSoleFrame(frame: ARFrame) {
        guard
            let originalPixelBuffer = frame.capturedImage.copy(),
            let depthPixelBuffer = frame.capturedDepthData?.depthDataMap.copy()
        else {
            return
        }
        let output = CaptureOutput(index: OutputConstants.soleCaptureOutputIndex,
                                   originalPixelBuffer: originalPixelBuffer,
                                   depthPixelBuffer: depthPixelBuffer,
                                   intrinsics: frame.camera.intrinsics,
                                   transform: frame.camera.transform)
        eventSubject.send(.captureOutput(output: output, capturePositionId: nil))
    }
}

// MARK: - Helpers
private extension CaptureServiceImpl {
    func currentCaptureTransform(camera: ARCamera) -> Int? {
        let cameraPosition = camera.transform.position
        
        for index in 0..<capturePositions.count {
            let position = capturePositions[index]
            if abs(position.position.x - cameraPosition.x) <= Constant.positionThreshold,
               abs(position.position.y - cameraPosition.y) <= Constant.positionThreshold,
               abs(position.position.z - cameraPosition.z) <= Constant.positionThreshold {
                return index
            }
        }
        return nil
    }
}

// MARK: - Constants
private struct Constant {
    static let circle = 2 * Float.pi
    static let radius: Float = 0.3
    static let yDistance: Float = 0.25
    static let xRotation: Float = .pi / 6
    
    static let positionThreshold: Float = 0.05
    static let rotationThreshold: Float = 0.02
}
