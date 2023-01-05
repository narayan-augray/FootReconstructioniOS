//
//  InstructionsViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import Combine

final class InstructionsViewModel: BaseViewModel {
    // MARK: - Properties
    private let outputs: [CaptureProcessedOutput]
    
    // MARK: - Transition
    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<InstructionsTransition, Never>()
    
    // MARK: - Init
    init(outputs: [CaptureProcessedOutput]) {
        self.outputs = outputs
        super.init()
    }
}

// MARK: - Navigation
extension InstructionsViewModel {
    func `continue`() {
        transitionSubject.send(.capture(outputs: outputs))
    }
}
