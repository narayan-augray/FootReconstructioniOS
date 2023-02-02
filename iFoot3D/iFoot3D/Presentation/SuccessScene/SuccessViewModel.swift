//
//  SuccessViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import Combine

final class SuccessViewModel: BaseViewModel {
    // MARK: - Publishers
    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<SuccessTransition, Never>()
    
    // MARK: - Navigation
    func scanAgain() {
        transitionSubject.send(.scanAgain)
    }
}
