import { createRouter, createWebHistory } from 'vue-router'
import HomeView from '../views/HomeView.vue'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      name: 'home',
      component: HomeView
    },
    {
      path: '/device',
      name: 'device',
      // route level code-splitting
      // this generates a separate chunk (About.[hash].js) for this route
      // which is lazy-loaded when the route is visited.
      component: () => import('../views/DeviceListView.vue')
    },
    {
      path: '/device/:name',
      name: 'device-detail',
      component: () => import('../views/DeviceDetailView.vue')
    }
  ]
})

export default router
