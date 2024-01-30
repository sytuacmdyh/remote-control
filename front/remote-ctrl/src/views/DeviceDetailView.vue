<script setup>
import { onMounted, reactive } from 'vue'
import { useRoute } from 'vue-router'
import { useDeviceStore } from '@/stores/device'
// import { storeToRefs } from 'pinia'

const deviceStore = useDeviceStore()
const { device, actionsAll, setDevice, bindAction, getRecentCommands } = deviceStore
// const { actionsAll } = storeToRefs(deviceStore)

const recentCommands = reactive([])

const refreshRecentCommands = () => {
  getRecentCommands().then((data) => {
    recentCommands.splice(0, recentCommands.length, ...data)
  })
}

const formatTimestamp = (timestamp_s) => {
  const date = new Date(timestamp_s * 1000)
  return `${date.getFullYear()}-${date.getMonth() + 1}-${date.getDate()} ${date.getHours()}:${date.getMinutes()}:${date.getSeconds()}`
}

onMounted(() => {
  // 从路由获取设备名
  const route = useRoute()
  const deviceName = route.params.name
  // 从设备仓库获取设备信息
  setDevice(deviceName)
})
</script>

<template>
  <v-container class="device-detail">
    <v-row>
      <v-col cols="12" sm="6" md="3">
        <v-card>
          <v-card-title>
            <h2>{{ device.name }}</h2>
          </v-card-title>
          <v-divider></v-divider>
          <v-card-text>
            <h5>{{ device.product }}</h5>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>
    <v-row class="mt-10">
      <v-col v-for="action in actionsAll" :key="action.name" sm="6" md="3">
        <v-card class="elevation-9" :title="action.nameCN" :subtitle="action.name">
          <v-form ref="createForm" @submit.prevent="bindAction(device, action)">
            <v-text-field
              v-model="action.addr"
              :rules="[(t) => (!isNaN(t) && t > 0) || '必须是数字']"
              clearable
              label="地址"
            ></v-text-field>
            <v-text-field
              v-model="action.cmd"
              :rules="[(t) => (!isNaN(t) && t > 0) || '必须是数字']"
              clearable
              label="指令"
            ></v-text-field>
            <v-btn type="submit" icon="mdi-check-bold"></v-btn>
          </v-form>
        </v-card>
      </v-col>
    </v-row>
    <v-row class="mt-10">
      <v-col>
        <v-card>
          <v-card-title>
            <h2>最近的命令</h2>
          </v-card-title>
          <v-divider></v-divider>
          <v-card-text>
            <v-btn icon="mdi-refresh" @click="refreshRecentCommands"></v-btn>
            <v-list>
              <v-list-item v-for="(cmd, idx) in recentCommands" :key="idx">
                <v-list-item-title>{{ cmd.addr }}_{{ cmd.cmd }}</v-list-item-title>
                <v-list-item-subtitle>{{ formatTimestamp(cmd.time) }}</v-list-item-subtitle>
              </v-list-item>
            </v-list>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>

<style></style>
