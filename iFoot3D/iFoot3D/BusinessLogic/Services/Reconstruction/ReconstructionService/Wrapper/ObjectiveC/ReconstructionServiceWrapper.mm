//
//  ReconstructionServiceWrapper.mm
//  iFoot3D
//
//  Created by Illia Khrypunov on 21.02.2023.
//

#import <Foundation/Foundation.h>
#import <opencv2/core.hpp>
#import "ReconstructionServiceWrapper.h"
#import "io.h"
#import "reconstruction.h"

@implementation ReconstructionServiceWrapper
// MARK: - Init
- (instancetype) init {
    self = [super init];
    return self;
}

// MARK: - Reconstruction
- (bool) reconstruct: (NSArray<NSArray<NSString *> *> *) rightSidePaths
       leftSidePaths: (NSArray<NSArray<NSString *> *> *) leftSide
           solePaths: (NSArray<NSArray<NSString *> *> *) sole
          outputPath: (NSString *) output {
    const char* outputPath = [output cStringUsingEncoding: NSUTF8StringEncoding];
    
    std::vector<std::vector<std::string>> rightPaths = [self getPaths:rightSidePaths];
    std::vector<std::vector<std::string>> leftPaths = [self getPaths:leftSide];
    std::vector<std::vector<std::string>> solePaths = [self getPaths:sole];
    
    [self printPaths:rightPaths];
    [self printPaths:leftPaths];
    [self printPaths:solePaths];
    
    std::vector<std::vector<std::vector<cv::Mat>>> input = ifoot3d::readMultipleInputData(rightPaths,
                                                                                          leftPaths,
                                                                                          solePaths);
    
    return ifoot3d::reconstructAndSaveLeg(input, outputPath);
}

- (bool) reconstruct: (NSString *) legPath
           solePaths: (NSArray<NSArray<NSString *> *> *) sole
      outputFolderPath: (NSString *) outputFolder {
    const char* legFilesPath = [legPath cStringUsingEncoding: NSUTF8StringEncoding];
    
    const char* outputPath = [outputFolder cStringUsingEncoding: NSUTF8StringEncoding];
    
    std::vector<int> rightSide {0, 1, 2, 3};
    std::vector<int> leftSide {0, 9, 8, 7, 6};
    
    std::vector<std::vector<std::string>> solePaths = [self getPaths:sole];
    
    std::vector<std::vector<std::vector<cv::Mat>>> input = ifoot3d::readMultipleInputData(legFilesPath,
                                                                                          rightSide,
                                                                                          leftSide,
                                                                                          solePaths,
                                                                                          outputPath);
    
    return ifoot3d::reconstructAndSaveLeg(input, outputPath);
}

// MARK: - Helpers
- (std::vector<std::vector<std::string>>) getPaths: (NSArray<NSArray<NSString *> *> *) paths {
    std::vector<std::vector<std::string>> output = {};
    
    for (NSArray<NSString *> * path in paths) {
        std::vector<std::string> outputPaths = {};
        
        for (NSString * string in path) {
            std::string path = [string cStringUsingEncoding: NSUTF8StringEncoding];
            outputPaths.push_back(path);
        }
        
        output.push_back(outputPaths);
    }
    
    return output;
}

- (void) printPaths: (std::vector<std::vector<std::string>>) paths {
    for (int i = 0; i < paths.size(); i++) {
        for (int j = 0; j < paths[i].size(); j++) {
            std::cout << paths[i][j] << std::endl;
        }
    }
    std::cout << std::endl;
}

@end
