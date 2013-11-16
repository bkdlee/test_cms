test_cms
========

Test Project CMS

This is a test project for Gladeye.

• Basic user account registration/management
• Facebook connectivity and authentication via OAuth
• Programatically sending emails via an SMTP server (i.e. using Gmail server)
• Generating a basic REST endpoint to interface with a database, sending back data as JSON

It is a fast project, without fully test.
The bugs I can assume:
1. edit user details must be edit password
2. Can not get email address from facebook
3. password can be only number

This proejct has two config files under src folder.
1. config.php
2. opauth.conf.php

In the config.php file has databse configration and SMTP email configration
In the opauth.conf.php file has facebook app_id and app_secret configration.

In apache, set DocumentRoot to "test_cms".
if want use "web" as root Directory, please remove "/web" under opauth.conf.php PATH

