require 'test_helper'

class DcountsControllerTest < ActionController::TestCase
  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:dcounts)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create dcount" do
    assert_difference('Dcount.count') do
      post :create, :dcount => { }
    end

    assert_redirected_to dcount_path(assigns(:dcount))
  end

  test "should show dcount" do
    get :show, :id => dcounts(:one).to_param
    assert_response :success
  end

  test "should get edit" do
    get :edit, :id => dcounts(:one).to_param
    assert_response :success
  end

  test "should update dcount" do
    put :update, :id => dcounts(:one).to_param, :dcount => { }
    assert_redirected_to dcount_path(assigns(:dcount))
  end

  test "should destroy dcount" do
    assert_difference('Dcount.count', -1) do
      delete :destroy, :id => dcounts(:one).to_param
    end

    assert_redirected_to dcounts_path
  end
end
