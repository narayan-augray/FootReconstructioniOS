//
//  BaseViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import Combine

class BaseViewModel: ViewModel {
    // MARK: - Cancellables
    var cancellables = Set<AnyCancellable>()
    
    // MARK: - Funcs
    func onViewDidLoad() {}
    func onViewWillAppear() {}
    func onViewDidAppear() {}
    func onViewWillDisappear() {}
    func onViewDidDisappear() {}
    
    // MARK: - Deinit
    deinit {
        log.debug(message: "deinit of \(String(describing: self))")
    }
}
