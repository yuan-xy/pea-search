class DcountsController < ApplicationController


  def index
    respond_to do |format|
      format.html { 
        if session[:yuan]=="xinyu" || params[:yuan]=="xinyu"
          @dcounts = Dcount.paginate(:order => genOrder, :page => params[:page], :per_page =>20)
          render
        else
          redirect_to "/404.html"
        end
        }
      format.js  { render :text => "#{Dcount.count+100}" }
    end
  end

  # POST /dcounts
  # POST /dcounts.xml
  def create
    @dcount = Dcount.new
    @dcount.ip = request.ip;
    @dcount.save
    render :text => "#{Dcount.count}"
  end

   private


  def genOrder
    s =" id desc"
  end
  
end
