//
//  ReusableCell.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import UIKit

protocol ReusableCell: AnyObject {
    static var reuseId: String { get }
}

extension ReusableCell {
    static var reuseId: String { String(describing: self) }
}

extension UITableViewCell: ReusableCell {}

extension UICollectionViewCell: ReusableCell {}
