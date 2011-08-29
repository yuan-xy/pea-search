function layer_menu(Lname,num)
{
  var L_menu_cnt=eval(Lname+".length");
  if(!L_menu_cnt)
  {
    var L_menu=eval(Lname+".style");
    if(L_menu.visibility != 'visible')
    {
      L_menu.visibility = 'visible';
    }else{
      L_menu.visibility = 'hidden';
    }
  }else{
    for(i=0; i<L_menu_cnt; i++)
    {
      var L_menu=eval(Lname+"["+i+"].style");

      if(i == num)
      {
        //
        if(L_menu.visibility != 'visible')
        {
          L_menu.visibility = 'visible';
        }else{
          L_menu.visibility = 'hidden';
        }
      }else{
        //
        L_menu.visibility='hidden';
      }
    }
  }
}