//
//  BaseViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import Combine

class BaseViewModel: ViewModel {
    // MARK: - Publishers
    private(set) lazy var errorPublisher = errorSubject.eraseToAnyPublisher()
    let errorSubject = PassthroughSubject<String, Never>()
    
    private(set) lazy var isLoadingPublisher = isLoadingSubject.eraseToAnyPublisher()
    let isLoadingSubject = PassthroughSubject<Bool, Never>()
    
    // MARK: - Cancellables
    var cancellables = Set<AnyCancellable>()
    
    // MARK: - Funcs
    func onViewDidLoad() {}
    func onViewWillAppear() {}
    func onViewDidAppear() {}
    func onViewWillDisappear() {}
    func onViewDidDisappear() {}
    
    // MARK: - Helpers
    func deleteFiles(fileUrls: [URL?]) {
        let fileManager = FileManager.default
        for fileUrl in fileUrls where fileUrl != nil {
            try? fileManager.removeItem(at: fileUrl!)
        }
    }
    
    // MARK: - Deinit
    deinit {
        log.debug(message: "deinit of \(String(describing: self))")
    }
}
