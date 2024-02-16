{
 "cells": [
  {
   "cell_type": "code",
   "execution_count": 1,
   "id": "8794cded",
   "metadata": {},
   "outputs": [],
   "source": [
    "import pandas as pd\n",
    "import numpy as np"
   ]
  },
  {
   "cell_type": "code",
   "execution_count": 2,
   "id": "afb82c37",
   "metadata": {},
   "outputs": [],
   "source": [
    "input_file = \"lazy_AS_deltaD1.csv\"\n",
    "df = pd.read_csv(\"./io_directory/\"+input_file,sep=\" \",header=None)"
   ]
  },
  {
   "cell_type": "code",
   "execution_count": 11,
   "id": "22dfd475",
   "metadata": {},
   "outputs": [],
   "source": [
    "#df"
   ]
  },
  {
   "cell_type": "code",
   "execution_count": 4,
   "id": "4d85252a",
   "metadata": {},
   "outputs": [],
   "source": [
    "df.drop(0,axis=1,inplace=True)"
   ]
  },
  {
   "cell_type": "code",
   "execution_count": 12,
   "id": "4db22083",
   "metadata": {},
   "outputs": [],
   "source": [
    "#df"
   ]
  },
  {
   "cell_type": "code",
   "execution_count": 6,
   "id": "9a3c1657",
   "metadata": {},
   "outputs": [],
   "source": [
    "df.sort_values(by=1,ascending=False,inplace=True)"
   ]
  },
  {
   "cell_type": "code",
   "execution_count": 13,
   "id": "33670127",
   "metadata": {},
   "outputs": [],
   "source": [
    "#df"
   ]
  },
  {
   "cell_type": "code",
   "execution_count": 8,
   "id": "6309136c",
   "metadata": {},
   "outputs": [],
   "source": [
    "df=df.reset_index(drop=True)"
   ]
  },
  {
   "cell_type": "code",
   "execution_count": 14,
   "id": "1b6fbb6b",
   "metadata": {},
   "outputs": [],
   "source": [
    "#df"
   ]
  },
  {
   "cell_type": "code",
   "execution_count": 10,
   "id": "034a9862",
   "metadata": {},
   "outputs": [
    {
     "data": {
      "text/plain": [
       "70.58"
      ]
     },
     "execution_count": 10,
     "metadata": {},
     "output_type": "execute_result"
    }
   ],
   "source": [
    "np.mean(df.index[df[2] == 1].tolist())\n"
   ]
  },
  {
   "cell_type": "code",
   "execution_count": null,
   "id": "468b1a73",
   "metadata": {},
   "outputs": [],
   "source": []
  }
 ],
 "metadata": {
  "kernelspec": {
   "display_name": "Python 3 (ipykernel)",
   "language": "python",
   "name": "python3"
  },
  "language_info": {
   "codemirror_mode": {
    "name": "ipython",
    "version": 3
   },
   "file_extension": ".py",
   "mimetype": "text/x-python",
   "name": "python",
   "nbconvert_exporter": "python",
   "pygments_lexer": "ipython3",
   "version": "3.8.10"
  }
 },
 "nbformat": 4,
 "nbformat_minor": 5
}
