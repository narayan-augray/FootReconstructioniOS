//
//  SceneDelegate.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 27.12.2022.
//

import UIKit

class SceneDelegate: UIResponder, UIWindowSceneDelegate {
    // MARK: - Properties
    var window: UIWindow?
    var appContainer: AppContainer!
    var appCoordinator: AppCoordinator!

    // MARK: - Funcs
    func scene(
        _ scene: UIScene,
        willConnectTo session: UISceneSession,
        options connectionOptions: UIScene.ConnectionOptions
    ) {
        guard let windowScene = (scene as? UIWindowScene) else { return }
        
        window = UIWindow(windowScene: windowScene)
        
        appContainer = AppContainerImpl()
        
        appCoordinator = AppCoordinator(window: window!, container: appContainer)
        appCoordinator.start()
    }
}
