This module provides an API for users to compose and send mails from
drupal site.

Send Mails is simple module to send mails from your drupal site.
It have a interface with to address field,subject field and Message Body
field like Gmail, Yahoo and other email service.

This module Support Full HTML messages there by we can send styled text
in your mails.

This module support to attach file in mails.
Note:
Maximum upload size is 5 MB 
Supported File formats in attachment are zip, tar, tgz, taz, z, gz, rar,
gif, png,jpg, jpeg, doc, xls, ppt, sxw, sxc, sxi, sdw, sdc, sdd, pdf.


Also we can send role based mails from this module.

Configuration:

1. Download the "Send Mails" module from drupal.org

2. Enable module by following standard Drupal module installation steps
(https://www.drupal.org/documentation/install/modules-themes/modules-7)

3. Give "Access Send Mails Service " permission for required users for
access the email send form.

4. Give "Access Advanced Send Mails Service" permission for Advanced
send mails options like, Role based sending. 

5. After configuration access the page "SITE_URL/send-mails/send" for
send emails.

6. Enable the Block "Send Mails Block" at "SITE_URL/admin/structure/block".

Optional Requirements:
1. Wysiwyg(module for style message body field)
path : https://www.drupal.org/project/wysiwyg
