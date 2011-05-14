var iHeight = 0;
var iTop = 0;
var clientHeight = 0;
// 取得当前页面显示所占用的高度
function getPageHeight() {
  if(document.body.clientHeight && document.documentElement.clientHeight) {  
    clientHeight = (document.body.clientHeight < document.documentElement.clientHeight) ? document.body.clientHeight : document.documentElement.clientHeight;          
  } else {  
    clientHeight = (document.body.clientHeight > document.documentElement.clientHeight) ? document.body.clientHeight : document.documentElement.clientHeight;      
  }

  iHeight = Math.max(document.body.scrollHeight,document.documentElement.scrollHeight);
}
// 判断滚动条是否到达底部
function reachBottom0() {
  var scrollTop = 0;
  if(document.documentElement && document.documentElement.scrollTop) {  
    scrollTop = document.documentElement.scrollTop;  
  } else if (document.body) {  
    scrollTop = document.body.scrollTop;  
  }
  if((scrollTop > 0) && (scrollTop + clientHeight == iHeight)) {
    return true;  
  } else {  
    return false; 
  }
}
function reachBottom() {
  iTop = document.documentElement.scrollTop + document.body.scrollTop;
  getPageHeight();
  if(((iTop+clientHeight)>parseInt(iHeight*0.95))||reachBottom0()) {
	return true;
  }
  return false;
}
