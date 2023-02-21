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
        //reconstruct()
        DispatchQueue.main.asyncAfter(deadline: .now() + 2.0) { [weak self] in
            self?.actionSubject.send(.reconstructionFailed)
        }
    }
    
    // MARK: - Navigation
    func capture() {
        transitionSubject.send(.capture)
    }
}

// MARK: - Reconstruction
private extension ProcessingViewModel {
    func reconstruct() {
        let input = prepareInput()
        
        let outputPath = FileManager.filePath(filename: ProcessingConstants.legScanFileName).path
        
        reconstructionService.reconstruct(rightSidePaths: input.right,
                                          leftSidePaths: input.left,
                                          solePaths: input.sole,
                                          outputPath: outputPath)
    }
    
    func prepareInput() -> (right: [[String]], left: [[String]], sole: [[String]]) {
        var right: [[String]] = []
        var left: [[String]] = []
        var sole: [[String]] = []
        
        for output in outputs {
            let paths: [String] = output.getFiles()
            
            if ProcessingConstants.leftSideOutputIndices.contains(output.index) {
                left.append(paths)
            }
            if ProcessingConstants.rightSideOutputIndices.contains(output.index) {
                right.append(paths)
            }
            if ProcessingConstants.sharedOutputIndices.contains(output.index) {
                left.append(paths)
                right.append(paths)
            }
            if output.index == OutputConstants.soleCaptureOutputIndex {
                sole.append(paths)
            }
        }
        
        return (right, left, sole)
    }
}

// MARK: - Private
private extension ProcessingViewModel {
    func setupBindings() {
        reconstructionService.eventPublisher
            .receive(on: DispatchQueue.main)
            .sink { [unowned self] (event) in
                switch event {
                case .reconstructed(let path):
                    deleteFiles(fileUrls: outputs.getFilesUrls())
                    
                    transitionSubject.send(.success(modelPath: path))
                }
            }
            .store(in: &cancellables)
    }
}
