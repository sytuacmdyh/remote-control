<script setup>
import { reactive, onMounted } from 'vue'
import { http_get } from '../request'

const sysInfo = reactive({})

const getSysInfo = () => {
  http_get('/api/rmt/info')
    .then((data) => {
      console.log(data);
      sysInfo.ip = data.ip
      sysInfo.version = data.version
      sysInfo.chip = data.chip
      sysInfo.sdk = data.sdk
    })
}

onMounted(() => {
  getSysInfo()
})
</script>

<template>
  <v-container>
    <v-row>
      <v-col cols="12" sm="6" md="3">
        <v-card>
          <v-card-title>
            <h2>控制台</h2>
          </v-card-title>
          <v-divider></v-divider>
          <v-card-text>
            <h5>欢迎使用智能红外遥控管理后台</h5>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>
    <v-row>
      <v-col cols="12" sm="6" md="3">
        <v-card>
          <v-card-title>
            <h4>基本信息</h4>
          </v-card-title>
          <v-divider></v-divider>
          <v-card-text>
            <v-row align="center">
              <v-col cols="3">IP: </v-col>
              <v-col cols="9">
                <v-text-field density="compact" v-model="sysInfo.ip" readonly></v-text-field>
              </v-col>
            </v-row>
            <v-row align="center">
              <v-col cols="3">版本: </v-col>
              <v-col cols="9">
                <v-text-field density="compact" v-model="sysInfo.version" readonly></v-text-field>
              </v-col>
            </v-row>
            <v-row align="center">
              <v-col cols="3">芯片: </v-col>
              <v-col cols="9">
                <v-text-field density="compact" v-model="sysInfo.chip" readonly></v-text-field>
              </v-col>
            </v-row>
            <v-row align="center">
              <v-col cols="3">SDK: </v-col>
              <v-col cols="9">
                <v-text-field density="compact" v-model="sysInfo.sdk" readonly></v-text-field>
              </v-col>
            </v-row>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>
