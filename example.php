<?php
//new the class
$ds = new DataServ();
//set iptions for mysql connect
$ds->sethost("127.0.0.1"); 
$ds->setuser("root");
$ds->setpasswd("root");
$ds->setdb("pw8");
$ds->settable("pw_house_agency");

/*set fields and value
* rand int like array("int",min,max)
* rand string like array("string",minlen,manlen)
* increment int like array("increment",minvalue,step) step default 1,minvalue default 0
* string like "admin"
* int like 0
*/
$ds->setfields(
	  array("agencyid"=>array("increment",0,2),
                "name"=>array("int",20,50),
                "address"=>array("string",20,50),
	        "icon"=>"icon",
                "bgimg"=>"bgimg",
	        "mainbusiness"=>1,
                "adminname"=>"admin",
		"isopen"=>1,
		"vieworder"=>1
          )
);
//create data file (start,end)
$ds->create(1,10000000);
//load data file into mysql
$ds->loaddata();
