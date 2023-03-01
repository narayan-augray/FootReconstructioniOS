//
//  ReconstructionService.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 21.02.2023.
//

import Foundation
import Combine

protocol ReconstructionService: AnyObject {
    // MARK: - Properties
    var eventPublisher: AnyPublisher<ReconstructionServiceEvent, Never> { get }
    
    // MARK: - Funcs
    func reconstruct(rightSidePaths: [[String]],
                     leftSidePaths: [[String]],
                     solePaths: [[String]],
                     outputPath: String)
    func reconstruct(legPath: String,
                     solePaths: [[String]],
                     outputPath: String)
}

final class ReconstructionServiceImpl: ReconstructionService {
    // MARK: - Properties
    private var reconstructor: ReconstructionServiceWrapper?
    
    private lazy var operationQueue: OperationQueue = {
        let queue = OperationQueue()
        queue.maxConcurrentOperationCount = 1
        queue.qualityOfService = .userInitiated
        queue.name = "reconstruction-service-queue"
        return queue
    }()
    
    // MARK: - Publishers
    private(set) lazy var eventPublisher = eventSubject.eraseToAnyPublisher()
    private let eventSubject = PassthroughSubject<ReconstructionServiceEvent, Never>()
    
    // MARK: - Funcs
    func reconstruct(
        rightSidePaths: [[String]],
        leftSidePaths: [[String]],
        solePaths: [[String]],
        outputPath: String
    ) {
        operationQueue.addOperation { [weak self] in
            guard let self = self else {
                return
            }
            
            self.reconstructor = ReconstructionServiceWrapper()
            
            guard
                let isSuccess = self.reconstructor?.reconstruct(
                    rightSidePaths,
                    leftSidePaths: leftSidePaths,
                    solePaths: solePaths,
                    outputPath: outputPath
                )
            else {
                self.eventSubject.send(.failure)
                return
            }
            
            self.eventSubject.send(isSuccess ? .reconstructed(path: outputPath) : .failure)
        }
    }
    
    func reconstruct(
        legPath: String,
        solePaths: [[String]],
        outputPath: String
    ) {
        operationQueue.addOperation { [weak self] in
            guard let self = self else {
                return
            }
            
            self.reconstructor = ReconstructionServiceWrapper()
            
            guard
                let isSuccess = self.reconstructor?.reconstruct(
                    legPath,
                    solePaths: solePaths,
                    outputPath: outputPath
                )
            else {
                self.eventSubject.send(.failure)
                return
            }
            
            self.eventSubject.send(isSuccess ? .reconstructed(path: outputPath) : .failure)
        }
    }
}
