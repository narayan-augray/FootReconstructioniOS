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
                     outputFolderPath: String)
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
                self.eventSubject.send(.failure(outputPath: nil))
                return
            }
            
            if isSuccess {
                self.eventSubject.send(.reconstructed(outputPath: outputPath))
            } else {
                self.eventSubject.send(.failure(outputPath: nil))
            }
        }
    }
    
    func reconstruct(
        legPath: String,
        solePaths: [[String]],
        outputFolderPath: String
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
                    logsFilePath: "\(outputFolderPath)/log.txt",
                    outputFolderPath: outputFolderPath
                )
            else {
                self.eventSubject.send(.failure(outputPath: outputFolderPath))
                return
            }
            
            if isSuccess {
                self.eventSubject.send(.reconstructed(outputPath: outputFolderPath))
            } else {
                self.eventSubject.send(.failure(outputPath: outputFolderPath))
            }
        }
    }
}
