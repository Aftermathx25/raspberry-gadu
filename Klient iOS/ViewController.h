//
//  ViewController.h
//  Klient iOS
//
//  Created by Aftermath on 03.01.2017.
//  Copyright © 2017 Aftermath. All rights reserved.
//

#import <UIKit/UIKit.h>

NSInputStream *inputStream;
NSOutputStream *outputStream;

@interface ViewController : UIViewController <NSStreamDelegate>
@property (weak, nonatomic) IBOutlet UITextField *textField;
@property (weak, nonatomic) IBOutlet UIView *joinView;
@property (weak, nonatomic) IBOutlet UITextField *passwordText;
@property (weak, nonatomic) IBOutlet UITextField *loginText;
@property (weak, nonatomic) IBOutlet UIView *loginView;
@property (weak, nonatomic) IBOutlet UITextField *odbiorca;
@property (weak, nonatomic) IBOutlet UITextField *doWyslania;
@property (weak, nonatomic) IBOutlet UIView *messageView;
@property (weak, nonatomic) IBOutlet UITextView *textArea;

- (IBAction)zalogujSie:(UIButton *)sender;
- (IBAction)wyslijDane:(UIButton *)sender;
- (IBAction)wyslijWiadomosc:(UIButton *)sender;

 

@end

