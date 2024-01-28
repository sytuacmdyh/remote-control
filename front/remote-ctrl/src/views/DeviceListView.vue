<script setup>
import { ref, onMounted, reactive } from 'vue'
import { http_get, http_post } from '../request'

const devices = reactive([])
const deviceName = ref('')
const productName = ref('')
const createForm = ref()
const rules = [
  (value) => {
    if (value) return true
    return '必填'
  }
]

const refreshDevices = () => {
  http_get('/api/rmt/getDeviceList').then((data) => {
    devices.splice(0, devices.length, ...data)
  })
}

const createDevice = () => {
  http_post('/api/rmt/createDevice', {
    name: deviceName.value,
    product: productName.value
  }).then(() => {
    createForm.value.reset()
    refreshDevices()
  })
}

const deleteDevice = (name) => {
  http_post('/api/rmt/deleteDevice', { name }).then(() => {
    refreshDevices()
  })
}

onMounted(() => {
  refreshDevices()
})
</script>

<template>
  <v-container class="device-list">
    <v-row>
      <v-col>
        <v-sheet elevation="3">
          <v-form ref="createForm" @submit.prevent="createDevice">
            <v-select
              v-model="productName"
              :rules="rules"
              label="产品"
              :items="['电灯', '空调']"
            ></v-select>
            <v-text-field v-model="deviceName" :rules="rules" label="设备名"></v-text-field>
            <v-btn type="submit" block class="mt-2">新建设备</v-btn>
          </v-form>
        </v-sheet>
      </v-col>
    </v-row>
    <v-row class="mt-10">
      <v-col v-for="device in devices" :key="device.name" sm="6" md="3">
        <v-card class="elevation-9" :title="device.name" :subtitle="device.product">
          <v-card-actions>
            <v-btn class="red" icon="mdi-delete" @click="deleteDevice(device.name)"></v-btn>
          </v-card-actions>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>

<style></style>
