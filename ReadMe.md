##HTTP Server built with C

This project is an assignment from UNT's Intro to Networks class.

##Instructions:
* compile server.c using gcc
* run compiled program "./a.out"
* go to proper url on machine and use the port 8080
	* ex: if using cse04.cse.unt.edu to run server, go to cse04.cse.unt.edu:8080
* server will serve the index.html file located in the resources directory
* to use the proxy, run the server again and go to cse04.cse.unt.edu:8080/www.unt.edu
* a file called www.unt.edu.html will be saved to the resources directory
* run the server again and go to the same url in your browser
* the cached html page will now be sent to the browser. 
	* you can tell by going to the bottom of the page and looking for "THIS IS CACHED"

##Approach and Limitations
My first 2 approaches were narrow-minded. Without knowing the full scope of the problem, I wrote programs that weren't modular, making handling different kinds of requests very difficult. It's been a while since I wrote in C, so I was missing and forgetting some key programming paradigms in my first attempts. My last attempt resulted in something that's easy to read and generally more stable. My second attempt had recursion and dynamically memory allocation that was only suitable for my local machine. 

I decided to create a resources directory to keep the html pages and server code serperate from each other. This also made it easier to write the code necessary to open those files. 

All cached web pages follow the naming scheme <domain name>.html. This makes it easy for the program to look up that file before requesting the webserver. 

This program was limited to only having a single thread, making it not very usable for real-world situations. I would have added threading to my program, but I wasn't confident with it, so I decided to keep it simple. 

###Screenshots:
####Serving index.html from resource directory
![alt tag](https://raw.githubusercontent.com/caseycarroll42/simpleCserver/master/serve.index.file.png)
####Serving file from webserver and caching it to resource directory
![alt tag](https://raw.githubusercontent.com/caseycarroll42/simpleCserver/master/cache.site.from.webserver.png)
####Serving cached html file
![alt tag](https://raw.githubusercontent.com/caseycarroll42/simpleCserver/master/serving.cached.file.png)


###References:
* tiny HTTPD: http://tinyhttpd.sourceforge.net
* HTTP Made Really Easy: http://www.jmarshall.com/easy/http/
* System Programming with C and Unix - Adam Hoover
