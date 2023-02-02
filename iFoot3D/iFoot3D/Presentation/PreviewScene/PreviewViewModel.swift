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

enum PreviewViewModelAction {
    case outputZipped(zipUrl: URL)
}

final class PreviewViewModel: BaseViewModel {
    // MARK: - Properties
    let objectUrl: URL
    let outputs: [CaptureProcessedOutput]
    var outputFilesUrls: [URL?] = []
    
    // MARK: - Transition
    private(set) lazy var actionPublisher = actionSubject.eraseToAnyPublisher()
    private let actionSubject = PassthroughSubject<PreviewViewModelAction, Never>()
    
    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<PreviewTransition, Never>()
    
    // MARK: - Init
    init(outputs: [CaptureProcessedOutput]) {
        self.outputs = outputs
        self.objectUrl = Bundle.main.url(forResource: Object.modelName, withExtension: Object.modelExtension)!
        super.init()
    }
    
    // MARK: - Navigation
    func close() {
        transitionSubject.send(.capture)
    }
    
    func success() {
        transitionSubject.send(.success)
    }
    
    // MARK: - Archive
    func zipOutput() {
        DispatchQueue.global().async { [weak self] in
            guard let self = self else { return }
            
            let fileUrls = self.outputs.getFilesUrls()
            self.outputFilesUrls.append(contentsOf: fileUrls)
            
            let archiveName = self.getArchiveName()
            
            do {
                let zipFilePath = try Zip.quickZipFiles(fileUrls.compactMap({ $0 }), fileName: archiveName)
                self.outputFilesUrls.append(zipFilePath)
                self.actionSubject.send(.outputZipped(zipUrl: zipFilePath))
            } catch {
                log.error(error: error)
                self.errorSubject.send(error.localizedDescription)
                self.isLoadingSubject.send(false)
                self.deleteFiles()
            }
        }
    }
    
    func deleteFiles() {
        let fileManager = FileManager.default
        for fileUrl in outputFilesUrls where fileUrl != nil {
            try? fileManager.removeItem(at: fileUrl!)
        }
    }
}

// MARK: - Helpers
private extension PreviewViewModel {
    func getArchiveName() -> String {
        let dateFormatter = DateFormatter()
        dateFormatter.dateFormat = "yyyy-MM-dd_HH:mm"
        return "\(UIDevice.modelName)_\(dateFormatter.string(from: Date()))"
    }
}
