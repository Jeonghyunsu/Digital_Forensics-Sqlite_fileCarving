# Sqlite_fileCarving

* Purpose

 Our initial goal was to restore the Webview Cache and visually prove that someone had accessed a certain page. 
However, we found that WebView Cache is stored as an image file or various resources, but it does not include the html that makes up the screen.

On the other hand, we knew that the Date information was stored in the WebView Cache, We learned that there is a history table that stores Date and url information in a specific app's Sqlite DataBase.
So we made a new screen using information from Webview Cache and Sqlite DataBase.
Also, because Sqlite DataBase File Carving works equally on all tables, we can use it in a variety of ways because we can get more information from a variety of tables as well as visit history tables.




* Reference

[1] http://forensicinsight.org/wp-content/uploads/2013/07/INSIGHT-SQLite-%EB%8D%B0%EC%9D%B4%ED%84%B0%EB%B2%A0%EC%9D%B4%EC%8A%A4-%EA%B5%AC%EC%A1%B0.pdf

[2] http://forensicsfromthesausagefactory.blogspot.kr/2011/04/carving-sqlite-databases-from.html

[3] https://sandersonforensics.com/forum/content.php?222-Recovering-deleted-records-from-an-SQLite-database

[4] http://www.ahnlab.com/kr/site/securityinfo/secunews/secuNewsView.do?curPage=1&menu_dist=2&seq=22324

[5] https://www.ahnlab.com/kr/site/securityinfo/secunews/secuNewsView.do?menu_dist=2&seq=22438

[6] http://www.cyber.pe.kr/2016/11/sqlite_22.html

[7] 오정훈, 이상진, “안드로이드 스마트폰 포렌식 분석 방법에 관한 연구”, 디지털 포렌식 연구, 제9호, 47~75, 2012.12
