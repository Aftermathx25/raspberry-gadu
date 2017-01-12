//
//  ViewController.m
//  Klient iOS
//
//  Created by Aftermath on 03.01.2017.
//  Copyright © 2017 Aftermath. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()

@end

@implementation ViewController 

UIAlertController* alert;
UIAlertController* alert2;
UIAlertController* alert3;
UIAlertController* alert4;
UIAlertAction* defaultAction;
UIAlertAction* defaultAction2;
UIAlertAction* defaultAction3;
UIAlertAction* defaultAction4;
NSString *pusty;
- (void)viewDidLoad {
    [super viewDidLoad];
    [self initNetworkCommunication];
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)initNetworkCommunication {
alert = [UIAlertController alertControllerWithTitle:@"Błąd" message:@"Błąd połączenia z hostem." preferredStyle:UIAlertControllerStyleAlert];
    defaultAction = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction * action){[self.view bringSubviewToFront:_joinView]; [_textField setText:@""];}];
    [alert addAction:defaultAction];
    alert2 = [UIAlertController alertControllerWithTitle:@"Błąd" message:@"Nie udało się wysłać wiadomości." preferredStyle:UIAlertControllerStyleAlert];
    defaultAction2 = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction * action){}];
    [alert2 addAction:defaultAction2];
    alert3 = [UIAlertController alertControllerWithTitle:@"Informacja" message:@"Serwer zakończył połączenie." preferredStyle:UIAlertControllerStyleAlert];
    defaultAction3 = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction * action){[self.view bringSubviewToFront:_joinView]; [_textField setText:@""];}];
    [alert3 addAction:defaultAction3];
    alert4 = [UIAlertController alertControllerWithTitle:@"Informacja" message:@"Odbiorca nie może być pusty." preferredStyle:UIAlertControllerStyleAlert];
    defaultAction4 = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction * action){}];
    [alert4 addAction:defaultAction4];
    pusty = [NSString stringWithFormat:@""];
}

- (void)stream:(NSStream *)theStream handleEvent:(NSStreamEvent)streamEvent {
    
    switch (streamEvent) {
            
        case NSStreamEventOpenCompleted:
            NSLog(@"Stream opened");
            
        case NSStreamEventHasBytesAvailable:
            if (theStream == inputStream) {
                uint8_t buffer[256];
                int len;
                while ([inputStream hasBytesAvailable]) {
                    len = (int)[inputStream read:buffer maxLength:sizeof(buffer)];
                    if (len > 0) {
                        NSString *output = [[NSString alloc] initWithBytes:buffer length:len encoding:NSUTF8StringEncoding];
                        if (nil != output){
                            NSLog(@"Server:%@", output);
                            if([output containsString:@"\nkoniec"]){
                                [self presentViewController:alert3 animated:YES completion:nil];
                                [inputStream close];
                                [outputStream close];
                                [inputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
                                [outputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
                                inputStream = nil;
                                outputStream = nil;
                                
                                [_textArea setText:pusty];
                                break;
                            }
                            if([output isEqualToString:(@"blad1\n")]){
                                [self presentViewController:alert2 animated:YES completion:nil];
                                NSString *informacja = [NSString stringWithFormat:@"Nie udało się wysłać\n"];
                                NSString *tekst = [NSString stringWithFormat:@"%@%@",_textArea.text,informacja];
                                [_textArea setText:tekst];
                            }
                            else
                            {
                            NSString *tekst = [NSString stringWithFormat:@"%@%@",_textArea.text,output];
                            [_textArea setText:tekst];
                            }
                        }
                        
                    }
                }
            }
            break;
        
        case NSStreamEventErrorOccurred:
            NSLog(@"Błąd połączenia z hostem.");
            [self presentViewController:alert animated:YES completion:nil];
            [inputStream close];
            [outputStream close];
            [inputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
            [outputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
            inputStream = nil;
            outputStream = nil;
            
            [_textArea setText:pusty];
            break;
        
        case NSStreamEventEndEncountered:
            [theStream close];
            [theStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
            break;
            
        default:
            //NSLog(@"Nieznany błąd");
            break;
            
    }
    
}

- (IBAction)zalogujSie:(UIButton *)sender {
    CFReadStreamRef is;
    CFWriteStreamRef os;
    CFStringRef ip = CFBridgingRetain(_textField.text);
    NSLog(@"IP jest równe:%@",ip);
    
    CFStreamCreatePairWithSocketToHost(NULL, ip, 1994, &is, &os);
   
    if(!CFReadStreamOpen(is)){
        NSLog(@"Error");
    }
    
    inputStream = (__bridge NSInputStream *)is;
    outputStream = (__bridge NSOutputStream *)os;
 
    [inputStream setDelegate:self];
    [outputStream setDelegate:self];
    [inputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [outputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [inputStream open];
    [outputStream open];
 

    [self.view bringSubviewToFront:_loginView];
   
    
}

- (IBAction)wyslijDane:(UIButton *)sender {

    NSString *login = _loginText.text;
    NSString *haslo = _passwordText.text;
    if(![_loginText hasText]){
       login = @"0";
    }
    if(![_passwordText hasText]){
        haslo = @"0";
    }
    NSData *data = [[NSData alloc] initWithData:[login dataUsingEncoding:NSUTF8StringEncoding]];
    [outputStream write:[data bytes] maxLength:[data length]];
    data = [[NSData alloc] initWithData:[haslo dataUsingEncoding:NSUTF8StringEncoding]];
    [outputStream write:[data bytes] maxLength:[data length]];
    [self.view bringSubviewToFront:_messageView];
}

- (IBAction)wyslijWiadomosc:(UIButton *)sender {
    
    
    if(![_odbiorca hasText]){
        [self presentViewController:alert4 animated:YES completion:nil];
    }
    else
    {
    NSString *wiadomosc = [NSString stringWithFormat:@"%@:%@\n\0", _odbiorca.text, _doWyslania.text];
        if(wiadomosc.length>8){
    NSLog(@"Wiadomosc:%@", wiadomosc);
    NSData *data = [[NSData alloc] initWithData:[wiadomosc dataUsingEncoding:NSUTF8StringEncoding]];
    [outputStream write:[data bytes] maxLength:[data length]];
    NSString *tekst = [NSString stringWithFormat:@"%@Ty do:%@",_textArea.text,wiadomosc];
    [_textArea setText:tekst];
        }
    }
}
@end




