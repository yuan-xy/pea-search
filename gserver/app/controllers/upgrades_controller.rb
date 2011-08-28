class UpgradesController < ApplicationController
  # GET /upgrades
  # GET /upgrades.xml
  def index
    @upgrades = Upgrade.all

    respond_to do |format|
      format.html # index.html.erb
      format.xml  { render :xml => @upgrades }
    end
  end

  # GET /upgrades/1
  # GET /upgrades/1.xml
  def show
    @upgrade = Upgrade.find(params[:id])

    respond_to do |format|
      format.html # show.html.erb
      format.xml  { render :xml => @upgrade }
    end
  end

  # GET /upgrades/new
  # GET /upgrades/new.xml
  def new
    @upgrade = Upgrade.new

    respond_to do |format|
      format.html # new.html.erb
      format.xml  { render :xml => @upgrade }
    end
  end

  # GET /upgrades/1/edit
  def edit
    @upgrade = Upgrade.find(params[:id])
  end

  # POST /upgrades
  # POST /upgrades.xml
  def create
    @upgrade = Upgrade.new(params[:upgrade])
    @upgrade.ip = request.ip;
    respond_to do |format|
      if @upgrade.save
        format.html { redirect_to(@upgrade, :notice => 'Upgrade was successfully created.') }
        format.xml  { render :xml => @upgrade, :status => :created, :location => @upgrade }
      else
        format.html { render :action => "new" }
        format.xml  { render :xml => @upgrade.errors, :status => :unprocessable_entity }
      end
    end
  end

  # PUT /upgrades/1
  # PUT /upgrades/1.xml
  def update
    @upgrade = Upgrade.find(params[:id])

    respond_to do |format|
      if @upgrade.update_attributes(params[:upgrade])
        format.html { redirect_to(@upgrade, :notice => 'Upgrade was successfully updated.') }
        format.xml  { head :ok }
      else
        format.html { render :action => "edit" }
        format.xml  { render :xml => @upgrade.errors, :status => :unprocessable_entity }
      end
    end
  end

  # DELETE /upgrades/1
  # DELETE /upgrades/1.xml
  def destroy
    @upgrade = Upgrade.find(params[:id])
    @upgrade.destroy

    respond_to do |format|
      format.html { redirect_to(upgrades_url) }
      format.xml  { head :ok }
    end
  end
end
