//
//  ProcessingViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 02.02.2023.
//

import Foundation
import Combine

enum ProcessingViewModelAction {
    case reconstructionFailed
}

final class ProcessingViewModel: BaseViewModel {
    // MARK: - Properties
    let outputs: [CaptureProcessedOutput]
    var input: ReconstructionInput!
    
    // MARK: - Services
    let reconstructionService: ReconstructionService
    
    // MARK: - Transition
    private(set) lazy var actionPublisher = actionSubject.eraseToAnyPublisher()
    private let actionSubject = PassthroughSubject<ProcessingViewModelAction, Never>()

    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<ProcessingTransition, Never>()
    
    // MARK: - Init
    init(outputs: [CaptureProcessedOutput],
         reconstructionService: ReconstructionService) {
        self.outputs = outputs
        self.reconstructionService = reconstructionService
        
        super.init()
        
        setupBindings()
    }
    
    // MARK: - Lifecycle
    override func onViewDidAppear() {
        super.onViewDidAppear()
        
        reconstruct(usingPathes: false)
    }
    
    // MARK: - Navigation
    func capture() {
        transitionSubject.send(.capture)
    }
}

// MARK: - Reconstruction
private extension ProcessingViewModel {
    func reconstruct(usingPathes: Bool) {
        input = prepareInput(usingPathes: usingPathes)
        
        switch input {
        case .pathes(let right, let left, let sole):
            let outputPath = FileManager.filePath(filename: ProcessingConstants.legScanFileName).path
            
            reconstructionService.reconstruct(
                rightSidePaths: right,
                leftSidePaths: left,
                solePaths: sole,
                outputPath: outputPath
            )
            
        case .combined(let path, let sole):
            let outputFolder = FileManager.createFolder(name: ProcessingConstants.outputFolderName)
            
            reconstructionService.reconstruct(
                legPath: path,
                solePaths: sole,
                outputFolderPath: outputFolder.path
            )
            
        default:
            break
        }
    }
}

// MARK: - Input
private extension ProcessingViewModel {
    func prepareInput(usingPathes: Bool) -> ReconstructionInput {
        if usingPathes {
            var right: [[String]] = []
            var left: [[String]] = []
            var sole: [[String]] = []
            
            for index in ProcessingConstants.rightSideOutputIndices {
                if let output = outputs.first(where: { $0.index == index }) {
                    right.append(output.getFiles())
                }
            }
            
            for index in ProcessingConstants.leftSideOutputIndices {
                if let output = outputs.first(where: { $0.index == index }) {
                    left.append(output.getFiles())
                }
            }
            
            for output in outputs.sorted(by: { $0.index < $1.index }) {
                if output.index == OutputConstants.soleCaptureOutputIndex {
                    sole.append(output.getFiles())
                }
            }
            
            return .pathes(right: right, left: left, sole: sole)
        } else {
            let legPath = outputs.first!.originalImageUrl
                .deletingLastPathComponent()
                .path + "/"
            
            var sole: [[String]] = []
            for output in outputs.sorted(by: { $0.index < $1.index }) {
                if output.index == OutputConstants.soleCaptureOutputIndex {
                    sole.append(output.getFiles())
                }
            }
            
            return .combined(path: legPath, sole: sole)
        }
    }
}

// MARK: - Private
private extension ProcessingViewModel {
    func setupBindings() {
        reconstructionService.eventPublisher
            .receive(on: DispatchQueue.main)
            .sink { [unowned self] (event) in
                switch event {
                case .reconstructed(let outputPath):
                    transitionSubject.send(.success(outputPath: outputPath,
                                                    outputs: outputs,
                                                    input: input))
                    
                case .failure(let outputPath):
                    errorSubject.send("Reconstuction failed")
                    
                    DispatchQueue.main.asyncAfter(deadline: .now() + 1.5) { [weak self] in
                        guard let self = self else {
                            return
                        }
                        let modelPath = Bundle.main.path(forResource: "foot", ofType: "obj")
                        self.transitionSubject.send(.success(outputPath: outputPath ?? modelPath!,
                                                             outputs: self.outputs,
                                                             input: self.input))
                    }
                }
            }
            .store(in: &cancellables)
    }
}
