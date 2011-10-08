class DcountsController < ApplicationController
  # GET /dcounts
  # GET /dcounts.xml
  def index
    render :text => "#{Dcount.count+100}"
  end

  # POST /dcounts
  # POST /dcounts.xml
  def create
    @dcount = Dcount.new
    @dcount.ip = request.ip;
    @dcount.save
    render :text => "#{Dcount.count}"
  end

end
