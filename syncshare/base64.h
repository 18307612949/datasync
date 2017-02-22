/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   base64.h
 * Author: tx
 *
 * Created on 2016年2月24日, 上午10:42
 */

#ifndef BASE64_H
#define BASE64_H
#include <string>
extern std::string base64_encode(unsigned char const* , unsigned int len);  
extern std::string base64_decode(std::string const& s);  



#endif /* BASE64_H */

