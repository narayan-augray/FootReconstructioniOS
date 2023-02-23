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
        reconstruct()
    }
    
    // MARK: - Navigation
    func capture() {
        transitionSubject.send(.capture)
    }
}

// MARK: - Reconstruction
private extension ProcessingViewModel {
    func reconstruct() {
        input = prepareInput()
        
        let outputPath = FileManager.filePath(filename: ProcessingConstants.legScanFileName).path
        
        reconstructionService.reconstruct(
            rightSidePaths: input.right,
            leftSidePaths: input.left,
            solePaths: input.sole,
            outputPath: outputPath
        )
    }
    
    func prepareInput() -> ReconstructionInput {
        var right: [[String]] = []
        var left: [[String]] = []
        var sole: [[String]] = []
        
        let rightIndices = [1, 2, 3, 4]
        let leftIndices = [1, 7, 6, 5]
        
        for index in rightIndices {
            if let output = outputs.first(where: { $0.index == index }) {
                right.append(output.getFiles())
            }
        }
        
        for index in leftIndices {
            if let output = outputs.first(where: { $0.index == index }) {
                left.append(output.getFiles())
            }
        }
        
        for output in outputs.sorted(by: { $0.index < $1.index }) {
            if output.index == OutputConstants.soleCaptureOutputIndex {
                sole.append(output.getFiles())
            }
        }
        
        return .init(right: right, left: left, sole: sole)
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
                    transitionSubject.send(.success(modelPath: path,
                                                    outputs: outputs,
                                                    input: input))
                    
                case .failure:
                    errorSubject.send("Reconstuction failed")
                    
                    DispatchQueue.main.asyncAfter(deadline: .now() + 1.5) { [weak self] in
                        guard let self = self else {
                            return
                        }
                        let modelPath = Bundle.main.path(forResource: "foot", ofType: "obj")
                        self.transitionSubject.send(.success(modelPath: modelPath ?? "",
                                                             outputs: self.outputs,
                                                             input: self.input))
                    }
                }
            }
            .store(in: &cancellables)
    }
}
