# Filters added to this controller apply to all controllers in the application.
# Likewise, all the methods added will be available for all controllers.

class ApplicationController < ActionController::Base
  helper :all # include all helpers, all the time
  protect_from_forgery # See ActionController::RequestForgeryProtection for details


  def admin_authorize
    if params[:yuan]=="xinyu"
	session[:yuan]="xinyu"
    else
      redirect_to "/404.html" unless session[:yuan]=="xinyu"
    end
  end

  def has_value(s)
    if s.nil?
      false
    elsif s.empty?
      false
    else
      true
    end
  end


  # Scrub sensitive parameters from your log
  # filter_parameter_logging :password
end
