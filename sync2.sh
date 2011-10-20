chmod +x gserver/public/dispatch.fcgi
zip gserver/public/PeaSrch.zip ReleaseWin/PeaSearch-1.0.19-x86.exe
rsync -avz --exclude '*.bak' --exclude '*.exe' --exclude '*.dmp' gserver dooo@www.1dooo.com:/home/dooo  
