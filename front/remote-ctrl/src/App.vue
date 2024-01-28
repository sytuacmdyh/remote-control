<script setup>
import { RouterView } from 'vue-router'
import { ref } from 'vue'
import { useErrorStore } from '@/stores/error'
import { storeToRefs } from 'pinia'

const errorStore = useErrorStore()
const { errorMessage, snackbar, timeout } = storeToRefs(errorStore)
const { showError } = errorStore

const drawer = ref(false)

const reboot = () => {
  showError('暂不支持重启')
}
</script>

<template>
  <v-app>
    <v-app-bar color="primary">
      <template v-slot:prepend>
        <v-app-bar-nav-icon @click.stop="drawer = !drawer"></v-app-bar-nav-icon>
      </template>

      <v-app-bar-title>智能红外遥控管理后台</v-app-bar-title>

      <template v-slot:append>
        <v-btn>
          <v-icon icon="mdi-dots-vertical"></v-icon>
          <v-menu activator="parent">
            <v-list>
              <v-list-item title="重启" @click="reboot"> </v-list-item>
            </v-list>
          </v-menu>
          <v-snackbar v-model="snackbar" :timeout="timeout">
            {{ errorMessage }}

            <template v-slot:actions>
              <v-btn color="white" variant="text" @click="snackbar = false"> X </v-btn>
            </template>
          </v-snackbar>
        </v-btn>
      </template>
    </v-app-bar>

    <v-navigation-drawer v-model="drawer" temporary>
      <v-list nav>
        <v-list-item to="/" title="控制台" prepend-icon="mdi-home"></v-list-item>
        <v-list-item to="/device" title="设备" prepend-icon="mdi-devices"></v-list-item>
      </v-list>
    </v-navigation-drawer>

    <v-main>
      <!-- <v-container> -->
      <RouterView />
      <!-- </v-container> -->
    </v-main>
  </v-app>
</template>

<style scoped></style>
