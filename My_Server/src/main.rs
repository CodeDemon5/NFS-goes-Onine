use std::net::TcpListener;
//use std::fmt::Display;
use std::io::BufReader;
use std::io::prelude::*;
use std::fs;
fn main() 
{
    let listener = TcpListener::bind("127.0.0.1:7878").unwrap();

    for stream in listener.incoming() {
        let mut stream = stream.unwrap();
        // println!("{:?}",stream);
        let buf_reader = BufReader::new(& mut stream);
        //let http_request :Vec<_> = buf_reader.lines().map(|line| line.unwrap()).take_while(|line| !line.is_empty()).collect();
        let request_line = buf_reader.lines().next().unwrap().unwrap();
        println!("{}",request_line);
    // println!("Request: {:#?}", http_request);
    //     println!("Connection established!");
    let path = &request_line[5..request_line.len()-9];
    println!("{}",path);
    if path.is_empty() || path == " " {
        let status_line = "HTTP/1.1 200 OK";
        let contents = fs::read_to_string("hello.html").unwrap();
        let length_ =contents.len();
        let response = format!{"{status_line}\r\nContent-Length:{length_}\r\n\r\n{contents}"};
        stream.write_all(response.as_bytes()).unwrap();
    }
    else if !path.is_empty() && path !=" "{
        let status_line = "HTTP/1.1 404 NOT FOUND";
        let contents = fs::read_to_string("not_found.html").unwrap();
        let length_ =contents.len();
        let response = format!{"{status_line}\r\nContent-Length:{length_}\r\n\r\n{contents}"};
        stream.write_all(response.as_bytes()).unwrap();
    }
    else {
        let status_line = "HTTP/1.1 500 INTERNAL SERVER ERROR";
        let contents = fs::read_to_string("error.html").unwrap();
        let length_ =contents.len();
        let response = format!{"{status_line}\r\nContent-Length:{length_}\r\n\r\n{contents}"};
        stream.write_all(response.as_bytes()).unwrap();
    
    }
    }
    
}