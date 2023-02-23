//
//  CaptureOutputManager.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import UIKit
import ARKit
import Combine

protocol CaptureOutputManager {
    // MARK: - Properties
    var eventPublisher: AnyPublisher<CaptureOutputManagerEvent, Never> { get }
    
    // MARK: - Funcs
    func getCaputredFrames() -> Int
    func processOutput(output: CaptureOutput, identified: Bool)
    func finishProcessing()
    func reset()
}

final class CaptureOutputManagerImpl: CaptureOutputManager {
    // MARK: - Properties
    private let outputSettings: CaptureOutputSettings
    
    private var capturedFrames: Int = 0
    private var processedOutputs: [CaptureProcessedOutput] = []
    
    private lazy var operationQueue: OperationQueue = {
        let queue = OperationQueue()
        queue.maxConcurrentOperationCount = 1
        queue.qualityOfService = .userInitiated
        queue.name = "output-processing-queue"
        return queue
    }()
    
    // MARK: - Publishers
    private(set) lazy var eventPublisher = eventSubject.eraseToAnyPublisher()
    private let eventSubject = PassthroughSubject<CaptureOutputManagerEvent, Never>()
    
    // MARK: - Init
    init(outputSettings: CaptureOutputSettings) {
        self.outputSettings = outputSettings
    }
    
    // MARK: - Public
    func getCaputredFrames() -> Int {
        return capturedFrames
    }
}

// MARK: - Process
extension CaptureOutputManagerImpl {
    func processOutput(output: CaptureOutput, identified: Bool) {
        capturedFrames += 1
        
        operationQueue.addOperation { [weak self] in
            let outputIdentifier = identified ? UUID().uuidString : "\(self?.capturedFrames ?? -1)"
            
            guard
                let self = self,
                let originalImage = UIImage(pixelBuffer: output.originalPixelBuffer),
                let dataValues = self.dataValues(dataMap: output.depthPixelBuffer),
                let textDataUrl = self.saveTextData(
                    text: String.initFromArray(array: dataValues),
                    filename: self.outputSettings.dataTextFileName,
                    identifier: outputIdentifier
                ),
                let originalImageUrl = self.saveImage(
                    image: originalImage,
                    filename: self.outputSettings.originalFileName,
                    identifier: outputIdentifier
                ),
                let calibrationDataUrl = self.saveTextData(
                    text: self.calibrationText(extrinsics: output.transform, intrinsics: output.intrinsics),
                    filename: self.outputSettings.calibrationTextFileName,
                    identifier: outputIdentifier
                )
            else {
                return
            }
            
            self.processedOutputs.append(.init(
                index: output.index,
                originalImageUrl: originalImageUrl,
                dataTextFileUrl: textDataUrl,
                calibrationTextFileUrl: calibrationDataUrl)
            )
        }
    }
}

// MARK: - Finish
extension CaptureOutputManagerImpl {
    func finishProcessing() {
        operationQueue.addOperation { [weak self] in
            self?.eventSubject.send(.processedOutputs(outputs: self?.processedOutputs ?? []))
            self?.processedOutputs.removeAll()
        }
    }
}

// MARK: - Reset
extension CaptureOutputManagerImpl {
    func reset() {
        capturedFrames = 0
    }
}

// MARK: - Data values
private extension CaptureOutputManagerImpl {
    func dataValues(dataMap: CVPixelBuffer?) -> [[Float32]]? {
        guard let depthDataMap = dataMap else {
            return nil
        }
        
        let depthWidth = CVPixelBufferGetWidth(depthDataMap)
        let depthHeight = CVPixelBufferGetHeight(depthDataMap)
        var depthArray = [[Float32]]()
        
        CVPixelBufferLockBaseAddress(depthDataMap, CVPixelBufferLockFlags(rawValue: 0))
        let floatBuffer = unsafeBitCast(CVPixelBufferGetBaseAddress(depthDataMap),
                                        to: UnsafeMutablePointer<Float32>.self)
        
        for y in 0...depthHeight - 1 {
            var distancesLine = [Float32]()
            for x in 0...depthWidth - 1 {
                let distanceAtXYPoint = floatBuffer[y * depthWidth + x]
                distancesLine.append(distanceAtXYPoint)
            }
            depthArray.append(distancesLine)
        }
        
        return depthArray
    }
}

// MARK: - Calibration text
private extension CaptureOutputManagerImpl {
    func calibrationText(
        extrinsics: simd_float4x4,
        intrinsics: simd_float3x3
    ) -> String {
        var result = ""
        
        result += "\(OutputConstants.intrinsicsKey):\n\(intrinsics.toString)\n\n"
        
        result += "\(OutputConstants.extrinsicsKey):\n\(extrinsics.toString)"
        
        return result
    }
}

// MARK: - Save
extension CaptureOutputManagerImpl {
    func saveImage(
        image: UIImage,
        filename: String,
        identifier: String
    ) -> URL? {
        guard let data = image.pngData() ?? image.jpegData(compressionQuality: 1) else {
            return nil
        }
        let fileUrl = FileManager.filePath(filename: "\(filename)_\(identifier).png")
        do {
            try data.write(to: fileUrl)
            return fileUrl
        } catch {
            log.error(error: error)
            return nil
        }
    }
    
    func saveTextData(
        text: String?,
        filename: String,
        identifier: String
    ) -> URL? {
        guard let text = text else {
            return nil
        }
        let fileUrl = FileManager.filePath(filename: "\(filename)_\(identifier).txt")
        do {
            try text.write(to: fileUrl, atomically: true, encoding: .utf8)
            return fileUrl
        } catch {
            log.error(error: error)
            return nil
        }
    }
    
    func saveBinaryData(values: [[Float32]], filename: String) -> URL? {
        let height = values.count
        let width = values.first?.count ?? 0
        
        let resultFilename = "\(filename)_\(capturedFrames)_\(width)x\(height)"
        let fileUrl = FileManager.filePath(filename: resultFilename)
        
        let result: [Float32] = Array(values.joined())
        let data = Data(bytes: result, count: result.count * MemoryLayout<Float32>.stride)
        do {
            try data.write(to: fileUrl)
            return fileUrl
        } catch {
            log.error(error: error)
            return nil
        }
    }
}
