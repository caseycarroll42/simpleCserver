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
