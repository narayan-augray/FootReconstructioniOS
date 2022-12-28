//
//  Font.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import UIKit.UIFont

enum Font {
    private static let futuraBold: UIFont = UIFont(name: "FuturaDemiC", size: 10)!
    private static let sfProTextBold: UIFont = UIFont(name: "SFProText-Bold", size: 10)!
    private static let sfProTextMedium: UIFont = UIFont(name: "SFProText-Medium", size: 10)!
    private static let sfProTextRegular: UIFont = UIFont(name: "SFProText-Regular", size: 10)!
    private static let sfProTextThin: UIFont = UIFont(name: "SFProText-Thin", size: 10)!
}

extension Font {
    static func futuraBold(_ size: CGFloat) -> UIFont {
        return self.futuraBold.withSize(size)
    }
    
    static func sfProTextBold(_ size: CGFloat) -> UIFont {
        return self.sfProTextBold.withSize(size)
    }
    
    static func sfProTextMedium(_ size: CGFloat) -> UIFont {
        return self.sfProTextMedium.withSize(size)
    }
    
    static func sfProTextRegular(_ size: CGFloat) -> UIFont {
        return self.sfProTextRegular.withSize(size)
    }
    
    static func sfProTextThin(_ size: CGFloat) -> UIFont {
        return self.sfProTextThin.withSize(size)
    }
}
