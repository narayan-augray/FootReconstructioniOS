//
//  ProcessingViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 02.02.2023.
//

import Foundation
import Combine

final class ProcessingViewModel: BaseViewModel {
    // MARK: - Properties
    let outputs: [CaptureProcessedOutput]
    
    // MARK: - Transition
    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<ProcessingTransition, Never>()
    
    // MARK: - Init
    init(outputs: [CaptureProcessedOutput]) {
        self.outputs = outputs
        super.init()
        process()
    }
}

// MARK: - Private
private extension ProcessingViewModel {
    func process() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 3.0) { [weak self] in
            guard let self = self else { return }
            self.transitionSubject.send(.success(outputs: self.outputs))
        }
    }
}
