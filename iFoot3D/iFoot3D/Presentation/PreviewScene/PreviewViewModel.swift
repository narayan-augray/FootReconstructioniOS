//
//  PreviewViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 02.02.2023.
//

import Foundation
import UIKit
import Combine
import Zip

final class PreviewViewModel: BaseViewModel {
    // MARK: - Properties
    var objectUrl: URL?
    let outputPath: String
    let outputs: [CaptureProcessedOutput]
    let input: ReconstructionInput
    
    // MARK: - Transition
    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<PreviewTransition, Never>()
    
    private(set) lazy var zipFileUrlPublisher = zipFileUrlSubject.eraseToAnyPublisher()
    private let zipFileUrlSubject = CurrentValueSubject<URL?, Never>(nil)
    
    // MARK: - Init
    init(
        outputPath: String,
        outputs: [CaptureProcessedOutput],
        input: ReconstructionInput
    ) {
        self.outputPath = outputPath
        self.outputs = outputs
        self.input = input
        
        super.init()
        
        self.objectUrl = self.getObjectUrl()
    }
    
    // MARK: - Navigation
    func close() {
        transitionSubject.send(.close)
    }
    
    func success() {
        transitionSubject.send(.success)
    }
    
    // MARK: - Helpers
    func deleteFiles() {
        deleteFiles(fileUrls: [zipFileUrlSubject.value])
    }
    
    func archive() {
        DispatchQueue.global().async { [weak self] in
            guard let self = self else { return }
            
            let archiveName = UUID().uuidString
            
            var files = [self.getInputFile()]
            files.append(contentsOf: self.outputs.getFilesUrls())
            
            if !self.outputPath.contains(ProcessingConstants.objectFileFormat) {
                files.append(contentsOf: FileManager.getContents(path: self.outputPath))
            }
            
            do {
                let zipFilePath = try Zip.quickZipFiles(files.compactMap({ $0 }), fileName: archiveName)
                self.zipFileUrlSubject.value = zipFilePath
                
                self.deleteFiles(fileUrls: files)
            } catch {
                log.error(error: error)
                self.zipFileUrlSubject.value = nil
                
                self.deleteFiles(fileUrls: files)
            }
        }
    }
}

// MARK: - Private
private extension PreviewViewModel {
    func getInputFile() -> URL? {
        let text = input.toString()
        let fileUrl = FileManager.filePath(filename: "input.txt")
        do {
            try text.write(to: fileUrl, atomically: true, encoding: .utf8)
            return fileUrl
        } catch {
            log.error(error: error)
            return nil
        }
    }
    
    func getObjectUrl() -> URL? {
        if outputPath.contains(ProcessingConstants.objectFileFormat) {
            return URL(fileURLWithPath: outputPath)
        } else {
            for fileUrl in FileManager.getContents(path: outputPath) {
                if fileUrl.path.contains(ProcessingConstants.objectFileFormat) {
                    return fileUrl
                }
            }
        }
        return nil
    }
}
