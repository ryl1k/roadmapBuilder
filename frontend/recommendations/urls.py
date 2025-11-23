from django.urls import path
from . import views

app_name = 'recommendations'

urlpatterns = [
    path('', views.index, name='index'),
    path('recommend/', views.get_recommendations, name='get_recommendations'),
    path('plan/<int:user_id>/', views.view_plan, name='view_plan'),
    path('catalog/', views.courses_catalog, name='catalog'),
]
